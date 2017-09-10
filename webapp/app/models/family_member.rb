class FamilyMember < ApplicationRecord
  has_many :triggers

  def to_s
    name
  end
end
